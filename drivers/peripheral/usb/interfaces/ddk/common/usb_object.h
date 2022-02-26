/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef USB_OBJECT_H
#define USB_OBJECT_H

#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sys/time.h>
#include "securec.h"
#include "hdf_base.h"
#include "hdf_slist.h"
#include "hdf_dlist.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_mutex.h"
#include "osal_sem.h"
#include "osal_thread.h"
#include "osal_time.h"
#include "osal_atomic.h"

#define MAX_OBJECT_ID       (0x7FFFFFFF)

typedef enum {
    /* request completed without error. */
    USB_REQUEST_COMPLETED,
    /* request completed with short data. */
    USB_REQUEST_COMPLETED_SHORT,
    /* request failed */
    USB_REQUEST_ERROR,
    /* the request timeout */
    USB_REQUEST_TIMEOUT,
    /* request was cancelled */
    USB_REQUEST_CANCELLED,
    /* request wall stalled */
    USB_REQUEST_STALL,
    /* Device was disconnected */
    USB_REQUEST_NO_DEVICE,
    /* Device sent more data than requested */
    USB_REQUEST_OVERFLOW,
} UsbRequestStatus;

typedef enum {
    USB_PIPE_DIRECTION_OUT = 0x00,
    USB_PIPE_DIRECTION_IN = 0x80,
} UsbPipeDirection;

typedef enum {
    USB_PIPE_TYPE_CONTROL = 0U,
    USB_PIPE_TYPE_ISOCHRONOUS = 1U,
    USB_PIPE_TYPE_BULK = 2U,
    USB_PIPE_TYPE_INTERRUPT = 3U,
} UsbPipeType;

typedef enum {
    USB_INTERFACE_STATUS_NORMAL,
    USB_INTERFACE_STATUS_ADD,
    USB_INTERFACE_STATUS_REMOVE,
    USB_INTERFACE_STATUS_OTHER,
} UsbInterfaceStatus;

struct UsbObject {
    int32_t objectId;
    struct DListHead entry;
};

#endif /* USB_OBJECT_H */
