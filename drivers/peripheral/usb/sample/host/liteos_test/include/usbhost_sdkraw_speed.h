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

#ifndef USB_HOST_SDK_RAW_SPEED_H
#define USB_HOST_SDK_RAW_SPEED_H

#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_atomic.h"
#include "usb_raw_api.h"
#include "data_fifo.h"

#define TEST_LENGTH             512
#define TEST_CYCLE              30
#define TEST_TIME               0xffffffff
#define TEST_PRINT_TIME         2
#define TEST_WRITE              true
#define TEST_READ               false
#define USB_MAX_INTERFACES      32
#define DATARATE                9600
#define CHARFORMAT              8
#define ACM_NW                  30
#define ACM_NR                  30
#define READ_BUF_SIZE           8192
#define USB_CTRL_SET_TIMEOUT    0
#define USB_PIPE_DIR_OFFSET     7

enum UsbSerialCmd {
    USB_SERIAL_OPEN = 0,
    USB_SERIAL_CLOSE,
    USB_SERIAL_SPEED,
};
struct UsbSpeedTest {
    int busNum;
    int devAddr;
    int ifaceNum;
    int writeOrRead;
    bool printData;
    int paramNum;
};

struct AcmDevice;
struct AcmDb {
    struct UsbRawRequest *request;
    struct AcmDevice *instance;
    uint8_t *buf;
    int len;
    int use;
};

struct SerialDevice {
    struct AcmDevice *acm;
    struct UsbCdcLineCoding lineCoding;
    struct OsalMutex lock;
    struct DataFifo readFifo;
};

struct UsbEndpoint {
    uint8_t addr;
    uint8_t interval;
    uint16_t maxPacketSize;
};

struct AcmDevice {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    uint8_t ctrlIface;
    uint8_t dataIface;
    struct UsbEndpoint *notifyEp;
    struct UsbEndpoint *dataInEp;
    struct UsbEndpoint *dataOutEp;
    struct UsbEndpoint *dataEp;
    struct UsbRawConfigDescriptor *config;
    struct AcmDb db[TEST_CYCLE];
    struct OsalMutex writeLock;
    struct OsalMutex readLock;
    struct UsbRawRequest *notifyReq;
    struct UsbRawRequest *readReq[ACM_NR];
    struct UsbRawRequest *writeReq;
    struct UsbRawRequest *ctrlReq;
    int dataSize;
    struct OsalMutex lock;
    UsbRawHandle *devHandle;
    struct UsbSession *session;
    struct SerialDevice *port;
    uint32_t nbIndex;
    uint32_t nbSize;
    int transmitting;
    uint8_t busNum;
    uint8_t devAddr;
    uint8_t interfaceCnt;
    uint8_t *notificationBuffer;
    uint8_t interfaceIndex[USB_MAX_INTERFACES];
    struct UsbCdcLineCoding lineCoding;
    struct OsalThread ioThread;
    struct OsalThread ioSendThread;
    bool busy;
};

#endif /* USB_HOST_SDK_RAW_SPEED_H */
