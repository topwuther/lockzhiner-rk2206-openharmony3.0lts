/*
 * hdf_log_adapter.h
 *
 * osal driver
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

#ifndef HDF_LOG_ADAPTER_H
#define HDF_LOG_ADAPTER_H

#include <linux/printk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define _HDF_FMT_TAG(TAG, LEVEL) "[" #LEVEL "/" #TAG "] "
#define HDF_FMT_TAG(TAG, LEVEL) _HDF_FMT_TAG(TAG, LEVEL)

#define HDF_LOGV_WRAPPER(fmt, args...) printk(KERN_DEBUG HDF_FMT_TAG(HDF_LOG_TAG, V) fmt "\r\n", ## args)

#define HDF_LOGD_WRAPPER(fmt, args...) printk(KERN_DEBUG HDF_FMT_TAG(HDF_LOG_TAG, D) fmt "\r\n", ## args)

#define HDF_LOGI_WRAPPER(fmt, args...) printk(KERN_INFO HDF_FMT_TAG(HDF_LOG_TAG, I) fmt "\r\n", ## args)

#define HDF_LOGW_WRAPPER(fmt, args...) printk(KERN_WARNING HDF_FMT_TAG(HDF_LOG_TAG, W) fmt "\r\n", ## args)

#define HDF_LOGE_WRAPPER(fmt, args...) printk(KERN_ERR HDF_FMT_TAG(HDF_LOG_TAG, E) fmt "\r\n", ## args)
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HDF_LOG_ADAPTER_H */

