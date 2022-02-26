/*
 * usb_pnp_notify.h
 *
 * usb pnp notify adapter of linux
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

#ifndef USB_PNP_NOTIFY_H
#define USB_PNP_NOTIFY_H

#include "hdf_base.h"
#include "hdf_dlist.h"
#include "hdf_usb_pnp_manage.h"
#include "osal_mutex.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define USB_PNP_NOTIFY_MSLEEP_TIME  (10)

#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif

typedef enum {
    USB_INFO_NORMAL_TYPE,
    USB_INFO_ID_TYPE,
    USB_INFO_DEVICE_ADDRESS_TYPE,
} UsbInfoQueryParaType;

typedef enum {
    USB_PNP_DEVICE_INIT_STATUS,
    USB_PNP_DEVICE_ADD_STATUS,
    USB_PNP_DEVICE_REMOVE_STATUS,
    USB_PNP_DEVICE_INTERFACE_STATUS,
} UsbPnpDeviceStatus;

struct UsbPnpDeviceInfo {
    int32_t id;
    struct OsalMutex lock;
    UsbPnpDeviceStatus status;
    struct DListHead list;
    bool interfaceRemoveStatus[USB_PNP_INFO_MAX_INTERFACES];
    struct UsbPnpNotifyMatchInfoTable info;
};

struct UsbInfoQueryPara {
    UsbInfoQueryParaType type;
    union {
        int32_t id;
        uintptr_t usbDevAddr;
        struct {
            int32_t devNum;
            int32_t busNum;
        };
    };
};

union UsbPnpDeviceInfoData {
    struct usb_device *usbDev;
    struct UsbPnpAddRemoveInfo *infoData;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* USB_PNP_NOTIFY_H */
