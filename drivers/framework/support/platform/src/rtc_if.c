/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "rtc_if.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "rtc_base.h"
#include "rtc_core.h"

#define HDF_LOG_TAG rtc_if

static char *g_rtcServiceName = "HDF_PLATFORM_RTC";

DevHandle RtcOpen()
{
    struct RtcHost *host = NULL;

    host = (struct RtcHost *)DevSvcManagerClntGetService(g_rtcServiceName);
    if (host == NULL) {
        HDF_LOGE("rtc get service name failed");
        return NULL;
    }
    return (DevHandle)host;
}

void RtcClose(DevHandle handle)
{
    (void)handle;
}

int32_t RtcReadTime(DevHandle handle, struct RtcTime *time)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcReadTime: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->ReadTime == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->ReadTime(host, time);
}

int32_t RtcWriteTime(DevHandle handle, const struct RtcTime *time)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcWriteTime: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (RtcIsInvalid(time) == RTC_TRUE) {
        HDF_LOGE("RtcWriteTime: time invalid");
        return HDF_ERR_INVALID_PARAM;
    }

    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->WriteTime == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->WriteTime(host, time);
}

int32_t RtcReadAlarm(DevHandle handle, enum RtcAlarmIndex alarmIndex, struct RtcTime *time)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcReadAlarm: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }

    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->ReadAlarm == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->ReadAlarm(host, alarmIndex, time);
}

int32_t RtcWriteAlarm(DevHandle handle, enum RtcAlarmIndex alarmIndex, const struct RtcTime *time)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcWriteAlarm: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (RtcIsInvalid(time) == RTC_TRUE) {
        HDF_LOGE("RtcWriteAlarm: time invalid");
        return HDF_ERR_INVALID_PARAM;
    }

    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->WriteAlarm == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->WriteAlarm(host, alarmIndex, time);
}

int32_t RtcRegisterAlarmCallback(DevHandle handle, enum RtcAlarmIndex alarmIndex, RtcAlarmCallback cb)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcRegisterAlarmCallback: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->RegisterAlarmCallback == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->RegisterAlarmCallback(host, alarmIndex, cb);
}

int32_t RtcAlarmInterruptEnable(DevHandle handle, enum RtcAlarmIndex alarmIndex, uint8_t enable)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcAlarmInterruptEnable: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->AlarmInterruptEnable == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->AlarmInterruptEnable(host, alarmIndex, enable);
}

int32_t RtcGetFreq(DevHandle handle, uint32_t *freq)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcGetFreq: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->GetFreq == NULL) {
        HDF_LOGE("RtcGetFreq: pointer null");
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->GetFreq(host, freq);
}

int32_t RtcSetFreq(DevHandle handle, uint32_t freq)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcSetFreq: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->SetFreq == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->SetFreq(host, freq);
}

int32_t RtcReset(DevHandle handle)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcReset: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->Reset == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->Reset(host);
}

int32_t RtcReadReg(DevHandle handle, uint8_t usrDefIndex, uint8_t *value)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcReadReg: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->ReadReg == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->ReadReg(host, usrDefIndex, value);
}

int32_t RtcWriteReg(DevHandle handle, uint8_t usrDefIndex, uint8_t value)
{
    struct RtcHost *host = NULL;

    if (handle == NULL) {
        HDF_LOGE("RtcReadReg: handle is null");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = (struct RtcHost *)handle;
    if (host->method == NULL || host->method->WriteReg == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return host->method->WriteReg(host, usrDefIndex, value);
}
