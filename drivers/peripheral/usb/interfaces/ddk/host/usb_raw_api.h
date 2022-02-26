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

#ifndef USB_RAW_API_H
#define USB_RAW_API_H

#include "usb_ddk.h"
#include "usb_session.h"

#define USB_MAXINTERFACES   32

typedef void *UsbRawDevice;
typedef void *UsbRawHandle;
typedef void (*UsbRawRequestCallback)(const void *requestArg);

struct UsbControlRequestData {
    uint8_t requestType;
    uint8_t requestCmd;
    uint16_t value;
    uint16_t index;
    uint16_t length;
    unsigned int timeout;
    unsigned char *data;
};

struct UsbRequestData {
    unsigned char endPoint;
    unsigned char *data;
    int length;
    int *requested;
    unsigned int timeout;
};

struct UsbRawDescriptorParam {
    uint8_t descType;
    uint8_t descIndex;
    int length;
};

struct UsbRawRequest {
    unsigned char *buffer;
    int length;
    int actualLength;
    UsbRequestStatus status;
    void *userData;
};

struct UsbRawFillRequestData {
    unsigned char endPoint;
    unsigned char *buffer;
    int length;
    int numIsoPackets;
    UsbRawRequestCallback callback;
    void *userData;
    unsigned int timeout;
};

struct UsbRawEndpointDescriptor {
    struct UsbEndpointDescriptor endpointDescriptor;
    const unsigned char *extra; /** Extra descriptors. */
    int extraLength;        /** Length of the extra descriptors, in bytes. */
};

struct UsbRawInterfaceDescriptor {
    struct UsbInterfaceDescriptor interfaceDescriptor;
    const struct UsbRawEndpointDescriptor *endPoint;   /** Array of endpoint descriptors. */
    const unsigned char *extra; /** Extra descriptors. */
    int extraLength;        /* Length of the extra descriptors, in bytes. Must be non-negative. */
};

struct UsbRawInterface {
    uint8_t numAltsetting;  /* The number of alternate settings that belong to this interface. */
    /* Array of interface descriptors. */
    const struct UsbRawInterfaceDescriptor altsetting[];
};

struct UsbRawConfigDescriptor {
    struct UsbConfigDescriptor configDescriptor;
    const struct UsbRawInterface *interface[USB_MAXINTERFACES];   /** Array of interfaces supported by this configuration. */
    const unsigned char *extra; /** Extra descriptors. */
    int extraLength;        /** Length of the extra descriptors, in bytes. Must be non-negative. */
};

int UsbRawInit(struct UsbSession **session);
int UsbRawExit(const struct UsbSession *session);
UsbRawHandle *UsbRawOpenDevice(const struct UsbSession *session, uint8_t busNum, uint8_t usbAddr);
int UsbRawCloseDevice(const UsbRawHandle *devHandle);
int UsbRawSendControlRequest(const struct UsbRawRequest *request, const UsbRawHandle *devHandle,
    const struct UsbControlRequestData *requestData);
int UsbRawSendBulkRequest(const struct UsbRawRequest *request, const UsbRawHandle *devHandle,
    const struct UsbRequestData *requestData);
int UsbRawSendInterruptRequest(const struct UsbRawRequest *request, const UsbRawHandle *devHandle,
    const struct UsbRequestData *requestData);
int UsbRawGetConfigDescriptor(const UsbRawDevice *rawDev, uint8_t configIndex, struct UsbRawConfigDescriptor **config);
void UsbRawFreeConfigDescriptor(const struct UsbRawConfigDescriptor *config);
int UsbRawGetConfiguration(const UsbRawHandle *devHandle, int *config);
int UsbRawSetConfiguration(const UsbRawHandle *devHandle, int config);
int UsbRawGetDescriptor(const struct UsbRawRequest *request, const UsbRawHandle *devHandle,
    const struct UsbRawDescriptorParam *param, const unsigned char *data);
UsbRawDevice *UsbRawGetDevice(const UsbRawHandle *devHandle);
int UsbRawGetDeviceDescriptor(const UsbRawDevice *rawDev, struct UsbDeviceDescriptor *desc);
int UsbRawClaimInterface(const UsbRawHandle *devHandle, int interfaceNumber);
int UsbRawReleaseInterface(const UsbRawHandle *devHandle, int interfaceNumber);
int UsbRawResetDevice(const UsbRawHandle *devHandle);
struct UsbRawRequest *UsbRawAllocRequest(const UsbRawHandle *devHandle, int isoPackets, int length);
int UsbRawFreeRequest(const struct UsbRawRequest *request);
int UsbRawFillBulkRequest(const struct UsbRawRequest *request, const UsbRawHandle *devHandle,
    const struct UsbRawFillRequestData *fillData);
int UsbRawFillControlSetup(const unsigned char *setup, const struct UsbControlRequestData *requestData);
int UsbRawFillControlRequest(const struct UsbRawRequest *request, const UsbRawHandle *devHandle,
    const struct UsbRawFillRequestData *fillData);
int UsbRawFillInterruptRequest(const struct UsbRawRequest *request, const UsbRawHandle *devHandle,
    const struct UsbRawFillRequestData *fillData);
int UsbRawFillIsoRequest(const struct UsbRawRequest *request, const UsbRawHandle *devHandle,
    const struct UsbRawFillRequestData *fillData);
int UsbRawSubmitRequest(const struct UsbRawRequest *request);
int UsbRawCancelRequest(const struct UsbRawRequest *request);
int UsbRawHandleRequests(const UsbRawHandle *devHandle);

#endif /* USB_RAW_API_H */
