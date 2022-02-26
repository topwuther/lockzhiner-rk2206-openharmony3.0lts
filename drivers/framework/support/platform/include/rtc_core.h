/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef RTC_CORE_H
#define RTC_CORE_H

#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_mutex.h"
#include "rtc_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct RtcHost;
struct RtcMethod;

struct RtcHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    struct RtcMethod *method;
    void *data;
};

struct RtcMethod {
    int32_t (*ReadTime)(struct RtcHost *host, struct RtcTime *time);
    int32_t (*WriteTime)(struct RtcHost *host, const struct RtcTime *time);
    int32_t (*ReadAlarm)(struct RtcHost *host, enum RtcAlarmIndex alarmIndex, struct RtcTime *time);
    int32_t (*WriteAlarm)(struct RtcHost *host, enum RtcAlarmIndex alarmIndex, const struct RtcTime *time);
    int32_t (*RegisterAlarmCallback)(struct RtcHost *host, enum RtcAlarmIndex alarmIndex, RtcAlarmCallback cb);
    int32_t (*AlarmInterruptEnable)(struct RtcHost *host, enum RtcAlarmIndex alarmIndex, uint8_t enable);
    int32_t (*GetFreq)(struct RtcHost *host, uint32_t *freq);
    int32_t (*SetFreq)(struct RtcHost *host, uint32_t freq);
    int32_t (*Reset)(struct RtcHost *host);
    int32_t (*ReadReg)(struct RtcHost *host, uint8_t usrDefIndex, uint8_t *value);
    int32_t (*WriteReg)(struct RtcHost *host, uint8_t usrDefIndex, uint8_t value);
};

int RtcSetHostMethod(struct RtcHost *host, struct RtcMethod *method);
struct RtcHost *RtcHostCreate(struct HdfDeviceObject *device);
void RtcHostDestroy(struct RtcHost *host);

static inline struct RtcHost *RtcHostFromDevice(struct HdfDeviceObject *device)
{
    return (device == NULL) ? NULL : (struct RtcHost *)device->service;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* RTC_CORE_H */
