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

#ifndef USB_INTERFACE_H
#define USB_INTERFACE_H

#include "usb_session.h"

#define USB_CTRL_INTERFACE_ID   0xFF

typedef enum {
    USB_REQUET_TARGET_DEVICE,
    USB_REQUEST_TARGET_INTERFACE,
    USB_REQUEST_TARGET_ENDPOINT,
    USB_REQUEST_TARGET_OTHER,
} UsbRequestTargetType;

typedef enum {
    USB_REQUEST_TYPE_STANDARD,
    USB_REQUEST_TYPE_CLASS,
    USB_REQUEST_TYPE_VENDOR,
} UsbControlRequestType;

typedef enum {
    USB_REQUEST_DIR_TO_DEVICE,
    USB_REQUEST_DIR_FROM_DEVICE,
} UsbRequestDirection;

typedef enum {
    USB_REQUEST_PARAMS_CTRL_TYPE,
    USB_REQUEST_PARAMS_DATA_TYPE,
} UsbRequestParamsType;

typedef enum {
    USB_REQUEST_TYPE_INVALID,
    USB_REQUEST_TYPE_DEVICE_CONTROL,
    USB_REQUEST_TYPE_PIPE_WRITE,
    USB_REQUEST_TYPE_PIPE_READ,
} UsbRequestPipeType;

typedef void *UsbInterfaceHandle;

struct UsbPipeInfo {
    uint8_t interfaceId;
    uint8_t pipeId;
    uint8_t pipeAddress;
    UsbPipeType pipeType;
    UsbPipeDirection pipeDirection;
    uint16_t maxPacketSize;
    uint8_t interval;
};

struct UsbPipe {
    struct UsbObject object;
    struct UsbPipeInfo info;
};

struct UsbInterfaceInfo {
    uint8_t interfaceIndex;
    uint8_t altSettings;
    uint8_t curAltSetting;
    uint8_t pipeNum;
    uint8_t interfaceClass;
    uint8_t interfaceSubClass;
    uint8_t interfaceProtocol;
};

struct UsbInterface {
    struct UsbObject object;
    struct UsbInterfaceInfo info;
};

struct UsbRequestCompInfo {
    UsbRequestPipeType type;
    unsigned char *buffer; /* the address of data buffer */
    uint32_t length; /* the length of data buffer */
    uint32_t actualLength; /* the actual length of the transferred data */
    UsbRequestStatus status;
    void *userData;
};

struct UsbRequest {
    struct UsbObject object;
    struct UsbRequestCompInfo compInfo;
}__attribute__((aligned(4)));

typedef void (*UsbRequestCallback)(struct UsbRequest *request);

struct UsbControlRequest {
    UsbRequestTargetType target;
    UsbControlRequestType reqType;
    UsbRequestDirection directon;
    uint8_t request;
    uint16_t value;   /* request specific value */
    uint16_t index;   /* request specific index */
    void *buffer;
    uint32_t length;
};

struct UsbRequestParamsData {
    int numIsoPackets;
    UsbRequestDirection directon;
    unsigned char *buffer;
    int length;
};

struct UsbRequestParams {
    uint8_t interfaceId;
    uint8_t pipeId;
    uint8_t pipeAddress;
    UsbRequestCallback callback;
    void *userData;
    unsigned int timeout;
    UsbRequestParamsType requestType;
    union {
        struct UsbControlRequest ctrlReq;
        struct UsbRequestParamsData dataReq;
    };
};

int32_t UsbInitHostSdk(struct UsbSession **session);
int32_t UsbExitHostSdk(const struct UsbSession *session);
const struct UsbInterface *UsbClaimInterface(const struct UsbSession *session, uint8_t busNum,
    uint8_t usbAddr, uint8_t interfaceIndex);
int UsbReleaseInterface(const struct UsbInterface *interfaceObj);
int UsbAddOrRemoveInterface(const struct UsbSession *session, uint8_t busNum, uint8_t usbAddr,
    uint8_t interfaceIndex, UsbInterfaceStatus status);
UsbInterfaceHandle *UsbOpenInterface(const struct UsbInterface *interfaceObj);
int32_t UsbCloseInterface(const UsbInterfaceHandle *interfaceHandle);
int32_t UsbSelectInterfaceSetting(const UsbInterfaceHandle *interfaceHandle, uint8_t settingIndex,
    struct UsbInterface **interfaceObj);
int32_t UsbGetPipeInfo(const UsbInterfaceHandle *interfaceHandle, uint8_t settingIndex,
    uint8_t pipeId, struct UsbPipeInfo *pipeInfo);
int32_t UsbClearInterfaceHalt(const UsbInterfaceHandle *interfaceHandle, uint8_t pipeAddress);
struct UsbRequest *UsbAllocRequest(const UsbInterfaceHandle *interfaceHandle, int isoPackets, int length);
int UsbFreeRequest(const struct UsbRequest *request);
int UsbSubmitRequestAsync(const struct UsbRequest *request);
int32_t UsbFillRequest(const struct UsbRequest *request, const UsbInterfaceHandle *interfaceHandle,
    const struct UsbRequestParams *params);
int UsbCancelRequest(const struct UsbRequest *request);
int UsbSubmitRequestSync(const struct UsbRequest *request);
int UsbMemTestTrigger(bool enable);

#endif /* USB_INTERFACE_H */
