/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_DEVICE_H
#define HDF_DEVICE_H

#include "devhost_service.h"
#include "device_token_if.h"
#include "hdf_device_desc.h"
#include "hdf_object.h"
#include "hdf_service_observer.h"
#include "hdf_dlist.h"
#include "osal_mutex.h"

struct HdfDeviceNode;

struct IHdfDevice {
    struct HdfObject object;
    int (*Attach)(struct IHdfDevice *, struct HdfDeviceNode *);
    void (*Detach)(struct IHdfDevice *, struct HdfDeviceNode *);
};

struct HdfDevice {
    struct IHdfDevice super;
    struct DListHead node;
    struct DListHead devNodes;
    uint16_t deviceId;
    uint16_t hostId;
};
void HdfDeviceConstruct(struct HdfDevice *device);
void HdfDeviceDestruct(struct HdfDevice *device);
struct HdfObject *HdfDeviceCreate(void);
void HdfDeviceRelease(struct HdfObject *object);
struct HdfDevice *HdfDeviceNewInstance(void);
void HdfDeviceFreeInstance(struct HdfDevice *device);
void HdfDeviceDelete(struct HdfSListNode *deviceEntry);

#endif /* HDF_DEVICE_H */
