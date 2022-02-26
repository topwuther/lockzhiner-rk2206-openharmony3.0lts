/*
 * watchdog_adapter.c
 *
 * linux watchdog driver adapter.
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

#include <asm/uaccess.h>
#include <asm/ioctls.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/version.h>
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "watchdog_if.h"
#include "watchdog_core.h"

#define HDF_LOG_TAG hdf_watchdog_adapter
#define WATCHDOG_NAME_LEN 20

static int WdtAdapterIoctlInner(struct file *fp, unsigned cmd, unsigned long arg)
{
    int ret = HDF_FAILURE;
    mm_segment_t oldfs;

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    if (fp->f_op->unlocked_ioctl) {
        ret = fp->f_op->unlocked_ioctl(fp, cmd, arg);
    }
    set_fs(oldfs);
    return ret;
}

static int32_t WdtOpenFile(struct WatchdogCntlr *wdt)
{
    char name[WATCHDOG_NAME_LEN] = {0};
    struct file *fp = NULL;
    mm_segment_t oldfs;

    if (wdt == NULL) {
        return HDF_FAILURE;
    }
    if (sprintf_s(name, WATCHDOG_NAME_LEN - 1, "/dev/watchdog%d", wdt->wdtId) < 0) {
        return HDF_FAILURE;
    }
    oldfs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(name, O_RDWR, 0600); /* 0600 : for open mode */
    if (IS_ERR(fp)) {
        HDF_LOGE("filp_open %s fail", name);
        set_fs(oldfs);
        return HDF_FAILURE;
    }
    set_fs(oldfs);
    wdt->priv = fp;
    return HDF_SUCCESS;
}

static void WdtAdapterClose(struct WatchdogCntlr *wdt)
{
    mm_segment_t oldfs;
    struct file *fp = (struct file *)wdt->priv;

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    if (!IS_ERR(fp) && fp) {
        (void)filp_close(fp, NULL);
    }
    set_fs(oldfs);
    wdt->priv = NULL;
}
static int32_t WdtAdapterStart(struct WatchdogCntlr *wdt)
{
    struct file *fp = NULL;
    unsigned long arg = WDIOS_ENABLECARD;

    if (wdt == NULL) {
        return HDF_FAILURE;
    }
    if (wdt->priv == NULL) {
        HDF_LOGE("cntlr is not opened");
        return HDF_FAILURE;
    }
    fp = wdt->priv;
    if (WdtAdapterIoctlInner(fp, WDIOC_SETOPTIONS, (unsigned long)&arg) != 0) {
        HDF_LOGE("WDIOC_SETOPTIONS WDIOS_ENABLECARD fail");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t WdtAdapterStop(struct WatchdogCntlr *wdt)
{
    struct file *fp = (struct file *)wdt->priv;
    unsigned long arg = WDIOS_DISABLECARD;

    if (fp == NULL) {
        return HDF_FAILURE;
    }
    if (WdtAdapterIoctlInner(fp, WDIOC_SETOPTIONS, (unsigned long)&arg) != 0) {
        HDF_LOGE("%s WDIOC_SETOPTIONS WDIOS_DISABLECARD fail", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t WdtAdapterFeed(struct WatchdogCntlr *wdt)
{
    struct file *fp = NULL;

    if (wdt->priv == NULL) {
        return HDF_FAILURE;
    }
    fp = (struct file *)wdt->priv;
    if (WdtAdapterIoctlInner(fp, WDIOC_KEEPALIVE, 0) != 0) {
        HDF_LOGE("WDIOC_KEEPALIVE fail");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static struct watchdog_device *WdtCoreDataToWdd(void *wdCoreData)
{
    /*
     * defined in watchdog_dev.c
     * struct watchdog_core_data {
     * struct kref kref;
     * struct cdev cdev;
     * struct watchdog_device *wdd;
     * ...
     * }
     */ 
    struct WdtCoreDataHead {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,182)
        struct device dev;
#else
        struct kref kref;
#endif
        struct cdev cdev;
        struct watchdog_device *wdd;
    };
    return ((struct WdtCoreDataHead *)wdCoreData)->wdd;
}

static int32_t WdtAdapterGetStatus(struct WatchdogCntlr *wdt, int32_t *status)
{
    struct file *fp = (struct file *)wdt->priv;
    struct watchdog_device *wdd = NULL;

    if (fp == NULL || fp->private_data == NULL) {
        return HDF_FAILURE;
    }
    wdd = WdtCoreDataToWdd(fp->private_data);
    if (wdd == NULL) {
        return HDF_FAILURE;
    }
    if (watchdog_active(wdd)) {
        HDF_LOGE("WDT is ACTIVE");
        *status = (int32_t)WATCHDOG_START;
    } else {
        HDF_LOGE("WDT is not ACTIVE");
        *status = (int32_t)WATCHDOG_STOP;
    }
    return HDF_SUCCESS;
}

static int32_t WdtAdapterSetTimeout(struct WatchdogCntlr *wdt, uint32_t seconds)
{
    struct file *fp = NULL;
    unsigned long arg;

    if (wdt->priv == NULL) {
        return HDF_FAILURE;
    }
    fp = (struct file *)wdt->priv;
    arg = seconds;
    if (WdtAdapterIoctlInner(fp, WDIOC_SETTIMEOUT, (unsigned long)&arg) != 0) {
        HDF_LOGE("WDIOC_SETTIMEOUT fail");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t WdtAdapterGetTimeout(struct WatchdogCntlr *wdt, uint32_t *seconds)
{
    struct file *fp = NULL;
    unsigned long arg;

    if (wdt->priv == NULL) {
        return HDF_FAILURE;
    }
    fp = (struct file *)wdt->priv;
    if (WdtAdapterIoctlInner(fp, WDIOC_GETTIMEOUT, (unsigned long)&arg) != 0) {
        HDF_LOGE("WDIOC_GETTIMEOUT fail");
        return HDF_FAILURE;
    }
    *seconds = arg;
    return HDF_SUCCESS;
}

static struct WatchdogMethod g_wdtMethod = {
    .getStatus = WdtAdapterGetStatus,
    .start = WdtAdapterStart,
    .stop = WdtAdapterStop,
    .setTimeout = WdtAdapterSetTimeout,
    .getTimeout = WdtAdapterGetTimeout,
    .feed = WdtAdapterFeed,
    .getPriv = WdtOpenFile,
    .releasePriv = WdtAdapterClose,
};

static int32_t HdfWdtBind(struct HdfDeviceObject *obj)
{
    struct WatchdogCntlr *wdt = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    HDF_LOGI("%s: entry", __func__);
    if (obj == NULL || obj->property == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    wdt = (struct WatchdogCntlr *)OsalMemCalloc(sizeof(*wdt));
    if (wdt == NULL) {
        HDF_LOGE("%s: malloc wdt fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetUint32 == NULL) {
        HDF_LOGE("%s: invalid drs ops!", __func__);
        OsalMemFree(wdt);
        return HDF_FAILURE;
    }
    ret = drsOps->GetUint16(obj->property, "id", &wdt->wdtId, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read wdtId fail, ret %d!", __func__, ret);
        OsalMemFree(wdt);
        return ret;
    }
    wdt->ops = &g_wdtMethod;
    wdt->device = obj;
    ret = WatchdogCntlrAdd(wdt);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: err add watchdog:%d", __func__, ret);
        OsalMemFree(wdt);
        return ret;
    }
    HDF_LOGI("%s: dev service %s init success!", __func__, HdfDeviceGetServiceName(obj));
    return HDF_SUCCESS;
}

static int32_t HdfWdtInit(struct HdfDeviceObject *obj)
{
    (void)obj;
    return HDF_SUCCESS;
}

static void HdfWdtRelease(struct HdfDeviceObject *obj)
{
    struct WatchdogCntlr *wdt = NULL;

    HDF_LOGI("%s: enter", __func__);
    if (obj == NULL) {
        return;
    }
    wdt = WatchdogCntlrFromDevice(obj);
    if (wdt == NULL) {
        return;
    }
    WatchdogCntlrRemove(wdt);
    OsalMemFree(wdt);
}

struct HdfDriverEntry g_hdfWdtchdog = {
    .moduleVersion = 1,
    .moduleName = "HDF_PLATFORM_WATCHDOG",
    .Bind = HdfWdtBind,
    .Init = HdfWdtInit,
    .Release = HdfWdtRelease,
};

HDF_INIT(g_hdfWdtchdog);
