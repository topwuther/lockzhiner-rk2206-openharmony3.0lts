/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef USBFN_REQUEST_H
#define USBFN_REQUEST_H

#include "usb_object.h"

typedef enum {
    USB_REQUEST_TYPE_INVALID,
    USB_REQUEST_TYPE_PIPE_WRITE,
    USB_REQUEST_TYPE_PIPE_READ,
} UsbFnRequestType;

struct UsbFnRequest {
    const struct UsbObject *obj;

    struct DListHead list;

    void             *buf;
    uint32_t          length;

    UsbFnRequestType type;
    UsbRequestStatus status;

    uint32_t         actual;
    void (*complete)(uint8_t pipe, struct UsbFnRequest *req);
    void             *context;
};

typedef void *UsbFnInterfaceHandle;

struct UsbFnRequest *UsbFnAllocCtrlRequest(UsbFnInterfaceHandle handle, uint32_t len);
struct UsbFnRequest *UsbFnAllocRequest(UsbFnInterfaceHandle handle, uint8_t pipe, uint32_t len);
int UsbFnFreeRequest(struct UsbFnRequest *req);
int UsbFnGetRequestStatus(struct UsbFnRequest *req, UsbRequestStatus *status);
int UsbFnSubmitRequestAsync(struct UsbFnRequest *req);
int UsbFnCancelRequest(struct UsbFnRequest *req);
int UsbFnSubmitRequestSync(struct UsbFnRequest *req, uint32_t timeout);

#endif /* USBFN_REQUEST_H */
