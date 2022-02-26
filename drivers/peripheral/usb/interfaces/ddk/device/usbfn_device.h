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

#ifndef USBFN_DEVICE_H
#define USBFN_DEVICE_H

#include "usb_object.h"
#include "device_resource_if.h"
#include "usb_ddk.h"
#include "usbfn_interface.h"

struct UsbFnDevice {
    struct UsbObject object;
    uint8_t numInterfaces;
};

typedef enum {
    USBFN_DESC_DATA_TYPE_PROP,
    USBFN_DESC_DATA_TYPE_DESC,
} UsbFnDescDataType;

struct UsbString {
    uint8_t                     id;
    const char                  *s;
};

struct UsbFnStrings {
    uint16_t                    language;
    struct UsbString            *strings;
};

struct UsbFnFunction {
    const char                 *funcName;
    struct UsbFnStrings        **strings;
    struct UsbDescriptorHeader **fsDescriptors;
    struct UsbDescriptorHeader **hsDescriptors;
    struct UsbDescriptorHeader **ssDescriptors;
    struct UsbDescriptorHeader **sspDescriptors;
};

struct UsbFnConfiguration {
    uint8_t                     configurationValue;
    uint8_t                     iConfiguration;
    uint8_t                     attributes;
    uint16_t                    maxPower;
    struct UsbFnFunction        **functions;
};

struct UsbFnDeviceDesc {
    struct UsbDeviceDescriptor *deviceDesc;
    struct UsbFnStrings          **deviceStrings;
    struct UsbFnConfiguration    **configs;
};

struct UsbFnDescriptorData {
    union {
        const struct DeviceResourceNode *property;
        struct UsbFnDeviceDesc *descriptor;
    };
    UsbFnDescDataType type;
};

const struct UsbFnDevice *UsbFnCreateDevice(const char *udcName, const struct UsbFnDescriptorData *descriptor);
int UsbFnRemoveDevice(struct UsbFnDevice *fnDevice);
const struct UsbFnDevice *UsbFnGetDevice(const char *udcName);
int UsbFnGetDeviceState(struct UsbFnDevice *fnDevice, UsbFnDeviceState *devState);
const struct UsbFnInterface *UsbFnGetInterface(struct UsbFnDevice *fnDevice, uint8_t interfaceIndex);
int UsbFnMemTestTrigger(bool enable);

#endif /* USBFN_DEVICE_H */
