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

#ifndef USBFN_INTERFACE_H
#define USBFN_INTERFACE_H

#include "usb_object.h"
#include "usbfn_request.h"

/* Interface class codes */
#define USB_INTERFACE_CLASS_UNSPEC              0x00
#define USB_INTERFACE_CLASS_AUDIO               0x01
#define USB_INTERFACE_CLASS_CDC                 0x02
#define USB_INTERFACE_CLASS_HID                 0x03
#define USB_INTERFACE_CLASS_PHYSICAL            0x05
#define USB_INTERFACE_CLASS_IMAGE               0x06
#define USB_INTERFACE_CLASS_PRINTER             0x07
#define USB_INTERFACE_CLASS_MASS_STORAGE        0x08
#define USB_INTERFACE_CLASS_HUB                 0x09
#define USB_INTERFACE_CLASS_CDC_DATA            0x0a
#define USB_INTERFACE_CLASS_SMARTCARD           0x0b
#define USB_INTERFACE_CLASS_FIRM_UPD            0x0c
#define USB_INTERFACE_CLASS_SECURITY            0x0d
#define USB_INTERFACE_CLASS_VIDEO               0x0e
#define USB_INTERFACE_CLASS_DIAGNOSTIC          0xdc
#define USB_INTERFACE_CLASS_WIRELESS            0xe0
#define USB_INTERFACE_CLASS_IAD                 0xef
#define USB_INTERFACE_CLASS_APP_SPEC            0xfe
#define USB_INTERFACE_CLASS_VENDOR              0xff

typedef enum {
    USBFN_STATE_BIND,
    USBFN_STATE_UNBIND,
    USBFN_STATE_ENABLE,
    USBFN_STATE_DISABLE,
    USBFN_STATE_SETUP,
    USBFN_STATE_SUSPEND,
    USBFN_STATE_RESUME,
} UsbFnDeviceState;

struct UsbFnCtrlRequest {
    uint8_t reqType;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed));

struct UsbFnEvent {
    struct UsbFnCtrlRequest *setup;
    UsbFnDeviceState        type;
    void                    *context;
};

struct UsbFnInterfaceInfo {
    uint8_t index;              /* the index number of this interface */
    uint8_t numPipes;           /* the number of pipes on this interface */
    uint8_t interfaceClass;     /* class code for this interface */
    uint8_t subclass;           /* subclass code for this interface */
    uint8_t protocol;           /* protocol code for this interface */
    uint8_t configIndex;        /* config number for this interface */
};

struct UsbFnInterface {
    const struct UsbObject    *object;
    struct UsbFnInterfaceInfo info;
};

struct UsbFnPipeInfo {
    uint8_t           id;
    UsbPipeType       type;
    UsbPipeDirection  dir;
    uint16_t          maxPacketSize;
    uint8_t           interval;
};

typedef void (*UsbFnEventCallback)(struct UsbFnEvent *event);
typedef int32_t (*UsbFnPropCallback)(const struct UsbFnInterface *intf, const char *name, const char *value);

struct UsbFnRegistInfo {
    const char *name;
    const char *value;
    UsbFnPropCallback getProp;
    UsbFnPropCallback setProp;
};

int UsbFnStartRecvInterfaceEvent(struct UsbFnInterface *interface, uint32_t eventMask,
    UsbFnEventCallback callback, void *context);
int UsbFnStopRecvInterfaceEvent(struct UsbFnInterface *interface);
UsbFnInterfaceHandle UsbFnOpenInterface(struct UsbFnInterface *interface);
int UsbFnCloseInterface(UsbFnInterfaceHandle handle);
int UsbFnGetInterfacePipeInfo(struct UsbFnInterface *interface, uint8_t pipeId, struct UsbFnPipeInfo *info);
int UsbFnRegistInterfaceProp(const struct UsbFnInterface *interface,
    const struct UsbFnRegistInfo *registInfo);
int UsbFnGetInterfaceProp(const struct UsbFnInterface *interface,
    const char *name, char *value);
int UsbFnSetInterfaceProp(const struct UsbFnInterface *interface,
    const char *name, const char *value);

#endif /* USBFN_INTERFACE_H */
