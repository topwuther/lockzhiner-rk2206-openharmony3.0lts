/*
 * devmgr_load.c
 *
 * device manager loader of linux
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

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include "devmgr_service_start.h"
#include "hdf_log.h"

static int __init DeviceManagerInit(void)
{
    int ret;

    HDF_LOGD("%s enter", __func__);
    ret = DeviceManagerStart();
    if (ret < 0) {
        HDF_LOGE("%s start failed %d", __func__, ret);
    } else {
        HDF_LOGD("%s start success", __func__);
    }
    return ret;
}

late_initcall(DeviceManagerInit);

