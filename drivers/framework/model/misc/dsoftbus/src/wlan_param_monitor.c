/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "wlan_param_monitor.h"

#include "hdf_dsoftbus_driver.h"
#include "hdf_log.h"
#include "module_manager.h"
#include "osal_time.h"
#include "osal_timer.h"

#define HDF_LOG_TAG "wlan_param_monitor"

#define WLAN_PARAM_REPORT_INTERVAL 1000

typedef enum {
    CMD_START_MONITOR = 0,
    CMD_STOP_MONITOR,
    CMD_MAX_INDEX
} Command;

typedef enum {
    EVENT_WLAN_PARAM = 0,
    EVENT_MAX_INDEX
} Event;

typedef struct {
    OSAL_DECLARE_TIMER(timer);
    bool isTimerStart;
} WlanParamMonitorCtrl;

typedef struct {
    uint32_t event;
    uint32_t value;
} ReportInfo;

static WlanParamMonitorCtrl g_wlanParamMonitorCtrl = {
    .isTimerStart = false,
};

static void WlanParamReportTimer(uintptr_t arg)
{
    ReportInfo info;
    struct HdfSBuf *data = NULL;

    (void)arg;
    info.event = EVENT_WLAN_PARAM;
    info.value = (uint32_t)OsalGetSysTimeMs();
    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        HDF_LOGE("get sbuf fail");
        return;
    }
    if (!HdfSbufWriteBuffer(data, (const void *)&info, sizeof(info))) {
        HDF_LOGE("sbuf write report value fail");
        HdfSBufRecycle(data);
        return;
    }
    HdfSoftbusBroadcastEvent(SOFTBUS_MODULE_WLAN_PARAM_MONITOR, data);
    HdfSBufRecycle(data);
}

static void ProcessStartMonitor(void)
{
    if (g_wlanParamMonitorCtrl.isTimerStart) {
        HDF_LOGE("wlan param monitor timer is already started");
        return;
    }
    if (OsalTimerCreate(&g_wlanParamMonitorCtrl.timer, WLAN_PARAM_REPORT_INTERVAL,
        WlanParamReportTimer, 0) != HDF_SUCCESS) {
        HDF_LOGE("create wlan param monitor timer fail");
        return;
    }
    if (OsalTimerStartLoop(&g_wlanParamMonitorCtrl.timer) != HDF_SUCCESS) {
        OsalTimerDelete(&g_wlanParamMonitorCtrl.timer);
        HDF_LOGE("start wlan param monitor timer fail");
        return;
    }
    g_wlanParamMonitorCtrl.isTimerStart = true;
}

static void ProcessStopMonitor(void)
{
    if (!g_wlanParamMonitorCtrl.isTimerStart) {
        HDF_LOGE("wlan param monitor timer is not started");
        return;
    }
    if (OsalTimerDelete(&g_wlanParamMonitorCtrl.timer) != HDF_SUCCESS) {
        HDF_LOGE("delete wlan param monitor timer fail");
    } else {
        g_wlanParamMonitorCtrl.isTimerStart = false;
    }
}

int32_t SoftbusWlanParamMonitorInit(struct HdfDeviceObject *device)
{
    (void)device;
    HDF_LOGI("SoftbusWlanParamMonitorInit init");
    return HDF_SUCCESS;
}

void SoftbusWlanParamMonitorDeinit(void)
{
    if (g_wlanParamMonitorCtrl.isTimerStart) {
        ProcessStartMonitor();
    }
}

void SoftbusWlanParamMonitorProcess(const struct HdfSBuf *reqData, struct HdfSBuf *rspData)
{
    uint32_t cmd;
    const void *data = NULL;
    uint32_t dataSize;

    (void)rspData;
    if (reqData == NULL) {
        HDF_LOGE("reqData is null");
        return;
    }
    if (!HdfSbufReadBuffer((struct HdfSBuf *)reqData, &data, &dataSize)) {
        HDF_LOGE("read command fail");
        return;
    }
    cmd = *((uint32_t *)data);
    HDF_LOGI("process command: %d", cmd);
    switch (cmd) {
        case CMD_START_MONITOR:
            ProcessStartMonitor();
            break;
        case CMD_STOP_MONITOR:
            ProcessStopMonitor();
            break;
        default:
            break;
    }
}